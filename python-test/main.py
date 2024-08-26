from matplotlib.widgets import Button
import numpy as np
import matplotlib.pyplot as plt
from datetime import datetime


class Component:
    components_dict = {}

    def __init__(self, pid):
        self.pid = pid
        self.fsms = {}
        self.flows = {}
        self.ports = {}
        self.events = [[], []]
        self.fig = None  
        self.axs = None

    def create_state_change_visualization(self, ax, times_m, states, times_s, title, yname, base_time_dt, details=None):
        state_colors = {}
        added_to_legend = []
        clicked_rectangle = {}

        # Convert time from string format to datetime
        times_m_dt = [datetime.strptime(time, '%H:%M:%S:%f') for time in times_m]
        times_s_dt = [datetime.strptime(time, '%H:%M:%S:%f') for time in times_s]

        # Calculate state durations
        durations = [(times_s_dt[i] - times_m_dt[i]).total_seconds() for i in range(len(times_m))] #Duration of state
        starts = [(time - base_time_dt).total_seconds() for time in times_m_dt]
        # Calculate gaps between states in microseconds
        gaps =[(starts[i + 1] - (starts[i] + durations[i])) * 1e6 for i in range(len(starts) - 1)]
        rect_texts = {}

        # Add bars for states
        for i, state in enumerate(states):
            duration = durations[i]
            #Random color
            color = state_colors.setdefault(state, f'C{len(state_colors) % 10}')
            #Add to legend only one element
            if state not in added_to_legend:
                ax.barh(yname, 0, color=color, label=state)  # Dummy bar for legend
                added_to_legend.append(state)
            rect = ax.barh(yname, duration, left=starts[i], color=color, edgecolor='black')[0]
            rect_texts[rect] = [
                f"{state} Start: {times_m[i]} \n End: {times_s[i]}",
                details[i] if details is not None else 'No arguments',
                f"Duration: {duration:.2f} seconds"
            ]

        # Add bars for gaps
        for i, gap in enumerate(gaps):
            if gap > 0:  # Only if gap is positive
                gap_start = starts[i] + durations[i]
                rect = ax.barh(yname, gap * 1e-6, left=gap_start, color='lightgray', edgecolor='black')[0]
                rect_texts[rect] = [
                    f"Gap Duration: {gap:.0f} µs"
                ]

        ax.legend(loc='upper left', bbox_to_anchor=(1, 1))
        ax.set_title(title)

        def on_click(event):
            # Check if the click event occurred within the plotting axes
            if event.inaxes == ax:
                x, y = event.xdata, event.ydata  # Get click coordinates

                # Loop through each bar and its corresponding text
                for rect, text in rect_texts.items():
                    # Check if the click is inside the current bar
                    if rect.contains(event)[0]:
                        # Toggle text display: show if not shown, remove if already shown
                        if rect in clicked_rectangle:
                            clicked_rectangle.pop(rect).remove()  # Remove existing text
                        else:
                            # Add text at click location
                            clicked_rectangle[rect] = ax.text(
                                x, y, "\n".join(text), ha='center', va='center', 
                                color='black', fontsize=8, fontweight='bold'
                            )
                        plt.draw()  # Update the plot

        ax.figure.canvas.mpl_connect('button_press_event', on_click)

    def create_state_occurrence_visualization(self, ax, times_m, states, title, yname, base_time_dt):
        state_colors = {}  # Store colors for each state
        added_to_legend = []  # Track which states are added to the legend
        visible_scatter_texts = {}  # Store currently visible scatter texts

        # Convert times to datetime and calculate seconds from base time
        times_m_dt = [datetime.strptime(time, '%H:%M:%S:%f') for time in times_m]
        events = [(time - base_time_dt).total_seconds() for time in times_m_dt]

        scatter_texts = {}  # Store text for each scatter point

        for i, state in enumerate(states):
            # Assign color to each state and create scatter plot points
            color = state_colors.setdefault(state, f'C{len(state_colors) % 10}')
            scatter = ax.scatter(events[i], 0, edgecolors='w', s=50, color=color, label=(
                f"{state}" if state not in added_to_legend else ""))
            scatter_texts[scatter] = f"{times_m[i]}"

            if state not in added_to_legend:
                added_to_legend.append(state)  # Add state to legend

        # Set y-axis properties and title
        ax.set_yticks([0])
        ax.set_yticklabels([yname])
        ax.set_title(title)
        ax.legend(loc='upper left', bbox_to_anchor=(1, 1))

        # Create annotation for displaying information on hover
        annotated_text = ax.annotate("", xy=(0, 0), xytext=(20, 20), textcoords="offset points",
                                    bbox=dict(boxstyle="round", fc="w"),
                                    arrowprops=dict(arrowstyle="->"))
        annotated_text.set_visible(False)  # Initially hide the annotation

        def hover(event):
            # Show or hide annotation on hover
            vis = annotated_text.get_visible()
            if event.inaxes == ax:
                for point, text in scatter_texts.items():
                    cont, ind = point.contains(event)
                    if cont:
                        pos = point.get_offsets()[ind["ind"][0]]
                        annotated_text.xy = pos
                        annotated_text.set_text(text)
                        annotated_text.set_visible(True)
                        ax.figure.canvas.draw_idle()
                        return
                if vis:
                    annotated_text.set_visible(False)
                    ax.figure.canvas.draw_idle()

        def on_zoom(event):
            # Update visible scatter texts when zooming
            xlim = ax.get_xlim()
            visible_scatter_texts.clear()
            for scatter, text in scatter_texts.items():
                scatter_pos = scatter.get_offsets()[0][0]
                if (scatter_pos >= xlim[0]) & (scatter_pos <= xlim[1]):
                    visible_scatter_texts[scatter] = text
            hide_button.set_active(True)

        def show_data(event=None):
            # Calculate and display statistics based on visible scatter texts
            parsed_times = [datetime.strptime(
                time, "%H:%M:%S:%f") for time in visible_scatter_texts.values()]
            print(title)
            print(f"Component PID: {self.pid}")
            differences = [(parsed_times[i + 1] - parsed_times[i]).total_seconds()
                        for i in range(len(parsed_times) - 1)]
            if len(differences) > 0:
                print(f" Nr: {len(differences) + 1}")
                print(f" Min: {min(differences)}")
                print(f" Max: {max(differences)}")
                print(f" Avg: {np.average(differences)}")
                print(f" Std_dev: {np.std(differences)}")
                print()
            else:
                print("Not enough data to analyse")
                print()

        # Create a button to trigger data analysis
        button_pos = [ax.get_position().x1, ax.get_position().y1, 0.1, 0.05]
        hide_button_ax = fig.add_axes(button_pos)
        hide_button = Button(hide_button_ax, 'Analyse',
                            color='lightblue', hovercolor='0.975')
        hide_button.on_clicked(show_data)

        # Connect hover and zoom events to their respective handlers
        ax.figure.canvas.mpl_connect("motion_notify_event", hover)
        ax.callbacks.connect('xlim_changed', on_zoom)


with open('../build/Debug/output.txt', 'r') as file:
    lines = file.readlines()
lines = [element.replace('\n', '') for element in lines]
lines = [element for element in lines if element.strip()]
lines = [lines[i:i + 6] for i in range(0, len(lines), 6)]
for line in lines:
    pid = line[2].split()[1]
    Component.components_dict[pid] = Component(pid)

for line in lines:
    pid = line[2].split()[1]
    name = line[0].replace("name: ", "")
    try:
        ts = line[1].split()[1]
    except IndexError:
        print(line)
    args = line[4].split()
    try:
        end = line[5].split()[1]
    except IndexError:
        end = ""
    if line[3] == "cat: state ":
        fsms = Component.components_dict.get(pid).fsms
        if name in fsms.keys():
            fsms[name][0].append(ts)
            fsms[name][1].append(args[1])
            fsms[name][2].append(end)
        else:
            fsms[name] = [[ts], [args[1]], [end]]

    if "cat: port flow " in line[3]:
        flows = Component.components_dict[pid].flows
        if args[2] in flows.keys():
            flows[args[2]][0].append(ts)
            flows[args[2]][1].append(name)
            flows[args[2]][2].append(end)
            flows[args[2]][3].append(f"{args[1]} {args[3]} {args[4]}")
        else:
            flows[args[2]] = [[ts], [name], [end],
                              [f"{args[1]} {args[3]} {args[4]}"]]
    if "cat: port packet" in line[3]:
        ports = Component.components_dict[pid].ports
        if len(args) > 3:
            if f"{name}({args[1]}) {args[2]}" in ports.keys():
                ports[f"{name}({args[1]}) {args[2]}"][0].append(ts)
                ports[f"{name}({args[1]}) {args[2]}"][1].append(args[3])
            else:
                ports[f"{name}({args[1]}) {args[2]}"] = [[ts], [args[3]]]
        else:
            if f"{name}({args[1]}) Server" in ports.keys():
                ports[f"{name}({args[1]}) Server"][0].append(ts)
                ports[f"{name}({args[1]}) Server"][1].append("Server")
            else:
                ports[f"{name}({args[1]}) Server"] = [[ts], ["Server"]]
    if "cat: event" in line[3]:
        events = Component.components_dict[pid].events
        cat = line[3].split()
        events[0].append(ts)
        events[1].append(f"{name}_{cat[2]}_{cat[3]}")

for comp in Component.components_dict.keys():
    fsms = Component.components_dict.get(comp).fsms
    flows = Component.components_dict.get(comp).flows
    ports = Component.components_dict.get(comp).ports
    events = Component.components_dict.get(comp).events

    subplots_num = len(fsms.keys()) + len(flows.keys()) + len(ports.keys()) + 1
    # X is shared, and one column with subplots_num rows
    Component.components_dict[comp].fig, Component.components_dict[comp].axs = plt.subplots(subplots_num, 1,
                                                                                            figsize=(12, 16),
                                                                                            sharex=True)

    fig = Component.components_dict[comp].fig
    axs = Component.components_dict[comp].axs

    fig.suptitle(f"Component PID: {comp}", fontsize=16)

    i = 0
    min_value_dt = datetime.strptime('23:59:59:999999', '%H:%M:%S:%f')
    for fsm in fsms.keys():
        min_value = datetime.strptime(fsms[fsm][0][0], '%H:%M:%S:%f')
        if min_value < min_value_dt:
            min_value_dt = min_value
        Component.components_dict[comp].create_state_change_visualization(axs[i], fsms[fsm][0], fsms[fsm][1],
                                                                          fsms[fsm][2], fsm, "state", min_value_dt)
        i += 1
    for flow in flows.keys():
        Component.components_dict[comp].create_state_change_visualization(axs[i], flows[flow][0], flows[flow][1],
                                                                          flows[flow][2], flow, "flows", min_value_dt,
                                                                          flows[flow][3])
        i += 1
    for port in ports.keys():
        Component.components_dict[comp].create_state_occurrence_visualization(axs[i], ports[port][0], ports[port][1],
                                                                              port.split()[0], port.split()[1],
                                                                              min_value_dt)
        i += 1
    Component.components_dict[comp].create_state_occurrence_visualization(axs[i], events[0], events[1], "Events",
                                                                          "event", min_value_dt)
    axs[-1].set_xlabel('Time in seconds')

plt.tight_layout()
plt.show()
